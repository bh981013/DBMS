select x.hometown1, x.nick
from(select r.hometown as hometown1, a.level as level1, a.nickname as nick
      from Trainer r, CatchedPokemon a
      where r.id = a.owner_id) x,
      (select t.hometown as hometown2, max(level) as max
      from Trainer t, CatchedPokemon c
      where t.id = c.owner_id
      group by t.hometown) y
where x.hometown1 = y.hometown2 and x.level1 = y.max
order by x.hometown1;
